# Abrir pastas de trabalho protegidas por senha
<!-- keywords: senha, protegido, protegida, criptografado, criptografia, abrir, bloqueado, bloqueada, senha incorreta, cancelar, xlsx, xls, ods, proteção de arquivo, senha padrão, privado, proteger com senha, senha do arquivo -->

Algumas pastas de trabalho são protegidas por senha ao serem salvas: sem ela, nenhum programa consegue ler o que está no arquivo. Isso é diferente de **Proteger Planilha** e **Proteger Pasta de Trabalho** (veja [Proteger uma planilha ou a pasta de trabalho](protect)), que apenas impedem alterações. Uma pasta de trabalho protegida por senha não pode ser aberta de forma alguma até que a senha seja digitada. Esta página explica como abrir uma delas e como criar uma.

## Abrir uma pasta protegida

Abra o arquivo como abre qualquer outro: pressione **{key:Open}**, escolha o arquivo ou arraste-o para a janela. O JeffCrunch percebe que o arquivo está protegido e pede a senha antes de ler qualquer coisa:

1. Digite a senha na caixa que diz **Este arquivo está protegido com senha.** As letras aparecem como pontos.
2. Clique em **Abrir**.

Se a senha não estiver correta, o JeffCrunch informa isso e pede novamente, quantas vezes você quiser. Clique em **Cancelar** para parar: nada é aberto, e as pastas de trabalho que você já tem abertas permanecem como estão. As senhas são exatas: letras maiúsculas, espaços e acentos fazem diferença.

O JeffCrunch não grava a senha em lugar nenhum. Ele a mantém na memória enquanto a pasta de trabalho está aberta, para que **Salvar** possa proteger o arquivo novamente, e a apaga quando você fecha a pasta de trabalho. Na próxima vez que você abrir o arquivo, ele pedirá a senha outra vez.

## O que pode ser aberto

- **Pastas de trabalho .xlsx e .xlsm** protegidas de uma das duas formas que o programa original usa desde 2007: a mais recente (a chave tem 128, 192 ou 256 bits, e o JeffCrunch também verifica se alguém alterou o arquivo depois de protegido) e a mais antiga (AES com chave de 128, 192 ou 256 bits).
- **Pastas de trabalho .xls** protegidas de qualquer uma das três formas desse formato: a cifragem simples dos arquivos mais antigos, a do tipo de 40 bits e a mais forte, que usa o serviço de chaves do próprio computador.
- **Planilhas .ods** protegidas por senha, em todas as três formas com que o LibreOffice e o OpenOffice as gravaram: a mais nova (o LibreOffice 25 e versões posteriores bloqueiam o arquivo inteiro com uma chave de 256 bits e geram a chave a partir da senha com um método propositalmente lento e que usa muita memória; por isso, abrir esse tipo de arquivo leva um pouco mais de tempo), a de 256 bits das versões anteriores e a mais antiga.

Arquivos .xls mais antigos têm limites quanto à própria senha. A cifragem simples dos arquivos mais antigos usa apenas os 15 primeiros caracteres de uma senha e só comporta letras, dígitos e sinais do conjunto de caracteres do Oeste europeu (Latim-1): uma senha com outras letras (como grego ou chinês) nunca é aceita como correta nesse caso. O tipo de 40 bits aceita qualquer letra, mas usa apenas os 16 primeiros caracteres. O JeffCrunch grava uma cópia .xls com senha no tipo mais forte (o que usa o serviço de chaves do computador, com chave de 128 bits), porque os outros dois são mais fracos. Esse tipo usa os 255 primeiros caracteres da senha, que podem ser quaisquer letras.

Alguns arquivos são bloqueados com uma senha que todos conhecem, para que possam ser abertos por qualquer pessoa, mas não sejam alterados por engano. O JeffCrunch abre esses arquivos sem pedir a senha.

## Quando um arquivo não pode ser aberto

- **"A senha está incorreta."** A senha digitada não é a que foi usada para proteger o arquivo. Tente novamente com atenção às maiúsculas e aos espaços; o JeffCrunch não consegue recuperar uma senha perdida.
- **"Este arquivo está protegido de um modo que este programa não consegue abrir."** O arquivo está bloqueado por outro meio que não uma senha (por exemplo, uma chave guardada em um certificado), ou em um formato que o JeffCrunch não lê.
- **"A proteção por senha deste arquivo exige mais memória ou tempo do que este programa permite, por isso ele não foi aberto."** Um arquivo do LibreOffice 25 ou posterior define quanta memória e tempo levam para verificar a senha. O JeffCrunch aceita até dez vezes o que o próprio LibreOffice define (640 MiB de memória), para que um arquivo feito para sobrecarregar o computador não consiga fazê-lo. Um arquivo gravado pelo próprio LibreOffice fica bem abaixo desse limite.
- **"O arquivo está danificado ou foi alterado depois de protegido."** A senha estava certa, mas o conteúdo do arquivo não é o que foi bloqueado: ele foi truncado, danificado ou alterado por outro programa. Copie-o novamente a partir do original.

## Salvar com senha

Clique em **Arquivo** e depois em **Proteger com Senha** (ou em **Revisão > Proteger com Senha**), digite a senha duas vezes e clique em **Proteger**. A pasta de trabalho será gravada protegida na próxima vez que você salvá-la: quem abrir o arquivo terá de digitar a senha. Para trocar a senha, escolha o mesmo comando e digite uma nova; para removê-la, clique em **Remover Senha** e salve.

Uma pasta de trabalho que você abriu com senha a mantém enquanto estiver aberta, então **Salvar** grava o arquivo protegido novamente com a mesma senha. O JeffCrunch guarda a senha na memória apenas até você fechar a pasta de trabalho e então a apaga. O arquivo aberto com senha não fica desprotegido por acidente: uma cópia salva como arquivo .xls é protegida com a mesma senha, no modo mais forte do .xls, e uma cópia salva como .ods ou .ots é protegida com ela do jeito que descreve o padrão OpenDocument, de modo que outros programas que seguem esses padrões possam abrir a cópia com a senha. Uma cópia em texto não pode ser protegida; por isso, o JeffCrunch avisa antes que a cópia não será protegida, e você pode cancelar.

Use uma senha longa e guarde-a em um lugar seguro: ninguém, nem nós, consegue abrir o arquivo sem ela.
