# Abrir pastas de trabalho protegidas por senha
<!-- keywords: senha, protegido, protegida, criptografado, criptografia, abrir, bloqueado, bloqueada, senha errada, cancelar, xlsx, xls, ods, proteção de arquivo, senha padrão, privado, privada -->

Algumas pastas de trabalho são protegidas por senha quando são salvas: sem ela, nenhum programa consegue ler o que está no arquivo. Isso é diferente de **Proteger Planilha** e **Proteger Pasta de Trabalho** (veja [Proteger uma planilha ou a pasta de trabalho](protect)), que apenas impedem alterações. Uma pasta de trabalho protegida por senha não pode ser aberta de forma alguma enquanto a senha não for digitada. Esta página explica como abrir uma delas e como criar uma.

## Abrir uma pasta de trabalho protegida

Abra o arquivo como abre qualquer outro: pressione **{key:Open}**, escolha o arquivo ou arraste-o para a janela. O JeffCrunch percebe que o arquivo está protegido e pede a senha antes de ler qualquer coisa:

1. Digite a senha na caixa que diz **Este arquivo está protegido com senha.** As letras aparecem como pontos.
2. Clique em **Abrir**.

Se a senha não estiver correta, o JeffCrunch informa isso e pede novamente, quantas vezes você quiser. Clique em **Cancelar** para parar: nada é aberto, e as pastas de trabalho que você já tem abertas continuam como estão. As senhas são exatas: letras maiúsculas, espaços e acentos fazem diferença.

O JeffCrunch não grava a senha em lugar nenhum. Ele a mantém na memória enquanto a pasta de trabalho está aberta, para que **Salvar** possa proteger o arquivo novamente, e a apaga quando você fecha a pasta de trabalho. Na próxima vez que abrir o arquivo, ele pedirá a senha outra vez.

## O que pode ser aberto

- **Pastas de trabalho .xlsx e .xlsm** protegidas de um dos dois modos usados pelo programa original desde 2007: o mais recente (a chave tem 128, 192 ou 256 bits, e o JeffCrunch também verifica se ninguém alterou o arquivo depois de protegido) e o mais antigo (AES com chave de 128, 192 ou 256 bits).
- **Pastas de trabalho .xls** protegidas de qualquer um dos três modos desse formato: a cifra simples dos arquivos mais antigos, a de 40 bits e a mais forte, que usa o serviço de chaves do próprio computador.
- **Planilhas .ods** protegidas por senha, em todos os três modos com que o LibreOffice e o OpenOffice as gravaram: a mais recente (o LibreOffice 25 e versões posteriores bloqueiam o arquivo inteiro com uma chave de 256 bits e geram a chave a partir da senha com um método deliberadamente lento e que usa muita memória, por isso abrir esse arquivo leva um pouco mais de tempo), a de 256 bits das versões anteriores e a mais antiga.

Os arquivos .xls mais antigos têm limites quanto à própria senha. A cifra simples dos arquivos mais antigos usa apenas os primeiros 15 caracteres da senha e só aceita letras, dígitos e sinais do conjunto de caracteres do Oeste europeu (Latin-1): uma senha com outras letras (como grego ou chinês) nunca é aceita como correta nesse caso. A cifra de 40 bits aceita qualquer letra, mas usa apenas os primeiros 16 caracteres. O JeffCrunch não grava arquivos .xls com senha.

Alguns arquivos são bloqueados com uma senha que todos conhecem, para que qualquer pessoa possa abri-los, mas não os altere por acidente. O JeffCrunch abre esses arquivos sem pedir senha.

## Quando um arquivo não pode ser aberto

- **"A senha não está correta."** A senha digitada não é a que foi usada para proteger o arquivo. Tente novamente com atenção às maiúsculas e aos espaços; o JeffCrunch não consegue recuperar uma senha perdida.
- **"Este arquivo está protegido de um modo que este programa não consegue abrir."** O arquivo está bloqueado por outro meio que não uma senha (por exemplo, uma chave guardada em um certificado), ou em um formato que o JeffCrunch não lê.
- **"A proteção por senha deste arquivo exige mais memória ou tempo do que este programa permite, por isso ele não é aberto."** Um arquivo criado pelo LibreOffice 25 ou versão posterior define quanta memória e tempo levará a verificação da senha. O JeffCrunch aceita até dez vezes o que o próprio LibreOffice define (640 MiB de memória), para que um arquivo feito para sobrecarregar o computador não consiga fazer isso. Um arquivo gravado pelo próprio LibreOffice fica bem dentro desse limite.
- **"O arquivo está danificado ou foi alterado depois de protegido."** A senha estava correta, mas o conteúdo do arquivo não é o que foi bloqueado: ele foi truncado, danificado ou alterado por outro programa. Copie-o novamente a partir do original.

## Salvar com senha

Clique em **Arquivo** e depois em **Proteger com Senha** (ou **Revisão > Proteger com Senha**), digite a senha duas vezes e clique em **Proteger**. A pasta de trabalho será gravada protegida na próxima vez que você salvar: quem abrir o arquivo precisará digitar a senha. Para trocar a senha, use o mesmo comando e digite uma nova; para removê-la, clique em **Remover Senha** e salve.

Uma pasta de trabalho aberta com senha mantém a proteção enquanto estiver aberta, por isso **Salvar** grava o arquivo protegido novamente com a mesma senha. O JeffCrunch mantém a senha na memória apenas até você fechar a pasta de trabalho e depois a apaga. O arquivo aberto com senha não fica desprotegido por acidente: uma cópia salva como arquivo .ods ou .ots fica protegida com a mesma senha, do modo descrito pelo padrão OpenDocument, de modo que outros programas que seguem esse padrão podem abri-la com a senha. Uma cópia em .xls ou em texto não pode ser protegida, por isso o JeffCrunch avisa antes que a cópia não será protegida, e você pode cancelar.

Use uma senha longa e guarde-a em um lugar seguro: ninguém, nem nós, consegue abrir o arquivo sem ela.
