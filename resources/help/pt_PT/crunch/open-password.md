# Abrir livros protegidos por palavra-passe
<!-- keywords: palavra-passe, senha, protegido, encriptado, encriptação, cifrado, abrir, bloqueado, palavra-passe errada, palavra-passe incorreta, cancelar, xlsx, xls, ods, proteção de ficheiro, palavra-passe predefinida, privado -->

Alguns livros são protegidos com uma palavra-passe quando são guardados: sem ela, nenhum programa consegue ler o conteúdo do ficheiro. Isto é diferente de **Proteger Folha** e **Proteger Livro** (consulte [Proteger uma folha ou o livro](protect)), que apenas impedem alterações. Um livro protegido com uma palavra-passe não pode ser aberto de todo enquanto a palavra-passe não for introduzida. Esta página explica como abrir um livro destes e como criar um.

## Abrir um livro protegido

Abra o ficheiro como abre qualquer outro: prima **{key:Open}**, escolha-o ou arraste-o para a janela. O JeffCrunch deteta que o ficheiro está protegido e pede a palavra-passe antes de ler qualquer coisa:

1. Escreva a palavra-passe na caixa com a indicação **Este ficheiro está protegido por palavra-passe.** As letras aparecem como pontos.
2. Clique em **Abrir**.

Se a palavra-passe não estiver correta, o JeffCrunch indica-o e pede-a novamente, as vezes que quiser. Clique em **Cancelar** para parar: nada é aberto e os livros que já tem abertos ficam como estão. As palavras-passe são exatas: as maiúsculas, os espaços e os acentos contam.

O JeffCrunch não escreve a palavra-passe em lado nenhum. Mantém-na em memória enquanto o livro está aberto, para que **Guardar** possa proteger novamente o ficheiro, e apaga-a quando fecha o livro. Da próxima vez que abrir o ficheiro, volta a pedi-la.

## O que pode ser aberto

- **Livros .xlsx e .xlsm** protegidos de qualquer uma das duas formas que o programa original utiliza desde 2007: a mais recente (a chave tem 128, 192 ou 256 bits e o JeffCrunch verifica também que ninguém alterou o ficheiro depois de ter sido protegido) e a mais antiga (AES com uma chave de 128, 192 ou 256 bits).
- **Livros .xls** protegidos de qualquer uma das três formas desse formato: a simples codificação dos ficheiros mais antigos, o tipo de 40 bits e o tipo mais forte, que utiliza o serviço de chaves do próprio computador.
- **Folhas de cálculo .ods** protegidas com uma palavra-passe, das três formas que o LibreOffice e o OpenOffice utilizaram: a mais recente (o LibreOffice 25 e posteriores bloqueiam o ficheiro inteiro com uma chave de 256 bits e geram a chave a partir da palavra-passe com um método propositadamente lento e que utiliza muita memória, pelo que abrir um ficheiro destes demora um pouco mais), o tipo de 256 bits das versões anteriores e o tipo mais antigo.

Os ficheiros .xls mais antigos têm limitações quanto à própria palavra-passe. A simples codificação dos ficheiros mais antigos utiliza apenas os primeiros 15 carateres de uma palavra-passe e só aceita letras, algarismos e sinais do conjunto de carateres ocidental (Latin-1): uma palavra-passe com outras letras (como gregas ou chinesas) nunca é aceite como correta nesses ficheiros. O tipo de 40 bits aceita quaisquer letras, mas apenas os primeiros 16 carateres. O JeffCrunch não escreve ficheiros .xls com palavra-passe.

Alguns ficheiros estão bloqueados com uma palavra-passe que todos conhecem, para que possam ser abertos por qualquer pessoa mas não alterados por engano. O JeffCrunch abre estes ficheiros sem perguntar.

## Quando não é possível abrir um ficheiro

- **"A palavra-passe não está correta."** A palavra-passe que escreveu não é aquela com que o ficheiro foi protegido. Tente novamente com atenção às maiúsculas e aos espaços; o JeffCrunch não consegue recuperar uma palavra-passe perdida.
- **"Este ficheiro está protegido de uma forma que este programa não consegue abrir."** O ficheiro está bloqueado por algo diferente de uma palavra-passe (por exemplo, uma chave associada a um certificado) ou num formato que o JeffCrunch não lê.
- **"A proteção por palavra-passe deste ficheiro exige mais memória ou tempo do que este programa permite, pelo que não é aberto."** Um ficheiro do LibreOffice 25 ou posterior define quanta memória e tempo são necessários para verificar a palavra-passe. O JeffCrunch aceita até dez vezes o que o próprio LibreOffice define (640 MiB de memória), para que um ficheiro criado para sobrecarregar o computador não o consiga fazer. Um ficheiro escrito pelo próprio LibreOffice fica bem dentro desse limite.
- **"O ficheiro está danificado ou foi alterado depois de ter sido protegido."** A palavra-passe estava correta, mas o conteúdo do ficheiro não corresponde ao que foi bloqueado: foi truncado, danificado ou alterado por outro programa. Copie-o novamente a partir do original.

## Guardar com uma palavra-passe

Clique em **Ficheiro** e, em seguida, em **Proteger com Palavra-passe** (ou **Rever > Proteger com Palavra-passe**), escreva a palavra-passe duas vezes e clique em **Proteger**. O livro é escrito protegido da próxima vez que o guardar: quem abrir o ficheiro terá de introduzir a palavra-passe. Para alterar a palavra-passe, escolha o mesmo comando e escreva uma nova; para a retirar, clique em **Remover Palavra-passe** e guarde.

Um livro que abriu com uma palavra-passe mantém-na enquanto está aberto, pelo que **Guardar** escreve novamente o ficheiro protegido com a mesma palavra-passe. O JeffCrunch mantém a palavra-passe em memória apenas até fechar o livro e, depois, apaga-a. O ficheiro que abriu com a palavra-passe não fica desprotegido por engano: se guardar uma cópia como ficheiro .xls ou .ods, que o JeffCrunch não consegue proteger, é avisado primeiro de que a cópia não ficará protegida e pode cancelar.

Utilize uma palavra-passe longa e guarde-a num local seguro: ninguém, incluindo nós, consegue abrir o ficheiro sem ela.
