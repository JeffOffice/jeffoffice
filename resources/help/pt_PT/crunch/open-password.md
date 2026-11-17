# Abrir livros protegidos com palavra-passe
<!-- keywords: palavra-passe, protegido, cifrado, encriptado, encriptação, abrir, bloqueado, palavra-passe errada, cancelar, xlsx, xls, ods, proteção de ficheiros, palavra-passe predefinida, privado -->

Alguns livros são protegidos com uma palavra-passe quando são guardados: sem ela, nenhum programa consegue ler o que está no ficheiro. Isto é diferente de **Proteger Folha** e **Proteger Livro** (ver [Proteger uma folha ou o livro](protect)), que apenas impedem alterações. Um livro protegido com palavra-passe não pode ser aberto enquanto a palavra-passe não for escrita. Esta página explica como abrir um destes livros e como criar um.

## Abrir um

Abra o ficheiro como abre qualquer outro: prima **{key:Open}**, escolha-o ou arraste-o para a janela. O JeffCrunch verifica que o ficheiro está protegido e pede a palavra-passe antes de ler seja o que for:

1. Escreva a palavra-passe na caixa que diz **Este ficheiro está protegido com uma palavra-passe.** Os carateres aparecem como pontos.
2. Clique em **Abrir**.

Se a palavra-passe não estiver correta, o JeffCrunch indica-o e volta a pedi-la, quantas vezes for necessário. Clique em **Cancelar** para parar: nada é aberto, e os livros que já tem abertos mantêm-se como estão. As palavras-passe são exatas: as maiúsculas, os espaços e os acentos contam.

O JeffCrunch não escreve a palavra-passe em lado nenhum. Mantém-na em memória enquanto o livro estiver aberto, para que **Guardar** possa voltar a proteger o ficheiro, e apaga-a quando fechar o livro. Na próxima vez que abrir o ficheiro, a palavra-passe volta a ser pedida.

## O que pode ser aberto

- **Livros .xlsx e .xlsm** protegidos com um dos dois métodos que o programa original utiliza desde 2007: o mais recente (a chave tem 128, 192 ou 256 bits, e o JeffCrunch verifica também que ninguém alterou o ficheiro depois de ser protegido) e o anterior (AES com uma chave de 128, 192 ou 256 bits).
- **Livros .xls** protegidos com qualquer um dos três métodos deste formato: a cifra simples dos ficheiros mais antigos, o tipo de 40 bits e o tipo mais forte, que utiliza o serviço de chaves do próprio computador.
- **Folhas de cálculo .ods** protegidas com palavra-passe, em todas as três formas como o LibreOffice e o OpenOffice as guardaram: a mais recente (o LibreOffice 25 e versões posteriores bloqueiam o ficheiro inteiro com uma chave de 256 bits e geram a chave a partir da palavra-passe com um método propositadamente lento e que utiliza muita memória, por isso abrir estes ficheiros demora um pouco mais), o tipo de 256 bits das versões anteriores e o tipo mais antigo.

Os ficheiros .xls mais antigos têm limites quanto à própria palavra-passe. A cifra simples dos ficheiros mais antigos utiliza apenas os primeiros 15 carateres de uma palavra-passe e só pode conter letras, algarismos e sinais do conjunto de carateres da Europa Ocidental (Latim-1): uma palavra-passe com outras letras (como grego ou chinês) nunca é aceite como correta nesse caso. O tipo de 40 bits aceita quaisquer letras, mas apenas os primeiros 16 carateres. O JeffCrunch não grava ficheiros .xls com palavra-passe.

Alguns ficheiros estão bloqueados com uma palavra-passe que todos conhecem, para que possam ser abertos por qualquer pessoa, mas não sejam alterados por acidente. O JeffCrunch abre-os sem perguntar.

## Quando um ficheiro não pode ser aberto

- **"A palavra-passe não está correta."** A palavra-passe que escreveu não é a que foi utilizada para proteger o ficheiro. Tente de novo, com atenção às maiúsculas e aos espaços; o JeffCrunch não consegue recuperar uma palavra-passe perdida.
- **"Este ficheiro está protegido de uma forma que este programa não consegue abrir."** O ficheiro está bloqueado por outro meio que não uma palavra-passe (por exemplo, uma chave guardada num certificado), ou numa forma que o JeffCrunch não lê.
- **"A proteção por palavra-passe deste ficheiro exige mais memória ou tempo do que este programa permite, por isso não foi aberto."** Um ficheiro criado pelo LibreOffice 25 ou posterior define a quantidade de memória e de tempo necessária para verificar a palavra-passe. O JeffCrunch aceita até dez vezes o que o próprio LibreOffice define (640 MiB de memória), para que um ficheiro feito para sobrecarregar o computador não consiga fazê-lo. Um ficheiro guardado pelo próprio LibreOffice fica bem dentro desse limite.
- **"O ficheiro está danificado ou foi alterado depois de ser protegido."** A palavra-passe estava correta, mas o conteúdo do ficheiro não é o que foi bloqueado: foi truncado, danificado ou alterado por outro programa. Copie-o novamente a partir do original.

## Guardar com palavra-passe

Clique em **Ficheiro** e depois em **Proteger com Palavra-passe** (ou **Rever > Proteger com Palavra-passe**), escreva a palavra-passe duas vezes e clique em **Proteger**. O livro fica protegido na próxima vez que o guardar: quem abrir o ficheiro terá de escrever a palavra-passe. Para a alterar, escolha o mesmo comando e escreva uma nova; para a retirar, clique em **Remover Palavra-passe** e guarde.

Um livro que abriu com palavra-passe mantém-na enquanto estiver aberto, por isso **Guardar** volta a gravar o ficheiro protegido com a mesma palavra-passe. O JeffCrunch mantém a palavra-passe em memória apenas até fechar o livro, e depois apaga-a. O ficheiro que abriu com a palavra-passe não fica desprotegido por acidente: uma cópia guardada como ficheiro .ods ou .ots fica protegida com a mesma palavra-passe, da forma descrita pela norma OpenDocument, pelo que outros programas que sigam a norma a podem abrir com a palavra-passe. Uma cópia em .xls ou em texto não pode ser protegida, por isso o JeffCrunch avisa primeiro que a cópia não ficará protegida, e pode cancelar.

Utilize uma palavra-passe longa e guarde-a num local seguro: ninguém, incluindo nós, consegue abrir o ficheiro sem ela.
