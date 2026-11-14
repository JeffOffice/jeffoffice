# Traduzir texto
<!-- keywords: traduzir, tradução, tradutor, idioma, idiomas, espanhol, francês, alemão, italiano, português, holandês, russo, ucraniano, indonésio, offline, sem internet, neste computador, baixar, modelos, substituir, inserir abaixo, controlar alterações -->

**Revisão > Idioma > Traduzir** abre o painel **Traduzir** à direita. Ele traduz seu texto no seu próprio computador. Suas palavras não são enviadas para lugar nenhum, e não é preciso conexão com a internet depois que os arquivos de tradução de um idioma estão no seu computador.

## Traduzir um texto

1. Selecione o texto que você quer traduzir. Se nada estiver selecionado, o painel usa o parágrafo em que está o cursor.
2. Clique em **Traduzir**, no grupo **Idioma**. O painel mostra seu texto em **Original**.
3. Em **De**, escolha o idioma em que o texto está. O JeffWrite começa com o idioma marcado no texto. Em **Para**, escolha o idioma desejado. O botão **Trocar** inverte os dois.
4. Clique em **Traduzir**. A tradução aparece em **Tradução**. Palavras em negrito, itálico e sublinhado continuam assim, e uma cor ou um tamanho são mantidos onde as palavras tinham a mesma aparência antes.
5. Clique em **Substituir** para colocar a tradução no lugar do seu texto, ou em **Inserir Abaixo** para adicioná-la em novos parágrafos abaixo do seu texto. Qualquer uma das opções é feita em uma única etapa, então **Desfazer** a desfaz. As palavras traduzidas são marcadas com o seu idioma, e a verificação ortográfica as confere nesse idioma.

Se você mover o cursor ou alterar o texto depois de clicar em **Traduzir**, clique em **Usar Seleção** para ler o texto novamente e clique em **Traduzir** outra vez. O painel não vai colocar uma tradução sobre um texto que não é mais o texto que foi traduzido.

Quando **Controlar Alterações** estiver ativado, **Substituir** e **Inserir Abaixo** são alterações controladas, como a sua digitação, para que um revisor veja o que havia antes. As palavras traduzidas então assumem a aparência que a sua digitação teria. Palavras que uma alteração controlada excluiu são deixadas como estão: não são traduzidas nem copiadas por **Inserir Abaixo**.

**Cancelar** interrompe uma tradução que esteja demorando demais, e fechar o painel também a interrompe: a janela nunca fica esperando por ela. Textos longos levam mais tempo: uma página de texto leva alguns segundos na maioria dos computadores. O painel traduz no máximo 1.000.000 de letras de uma vez (cerca de 200 páginas); se houver mais texto selecionado, ele avisa e pede que você selecione menos. Parágrafos longos são divididos em partes de cerca de 1.500 letras, no fim de uma frase sempre que possível, e depois são juntados novamente.

## A primeira vez que você usa um idioma

Uma tradução precisa de um conjunto de arquivos, chamado modelo, para cada direção: um para do inglês para o alemão, outro para do alemão para o inglês. Cada modelo tem cerca de 37 MB. Os arquivos não fazem parte do JeffWrite. Na primeira vez que você traduzir com um idioma, o painel informa o que é necessário e o tamanho, por exemplo "Baixar alemão (74 MB)?". Nada é baixado até que você clique nesse botão. O painel mostra o andamento do download, e **Cancelar** o interrompe e não deixa nada para trás.

O JeffWrite confere cada arquivo com uma impressão digital (uma soma de verificação SHA-256) antes de usá-lo. Um arquivo que não corresponde é descartado, e o idioma não é instalado.

Os arquivos ficam em uma única pasta compartilhada por todos os aplicativos do JeffOffice (uma pasta chamada JeffOffice na pasta de dados do seu usuário, em Translation/models). Assim, um idioma baixado em um aplicativo fica pronto nos outros, e é baixado apenas uma vez. Dois idiomas que não têm modelo próprio, como alemão e francês, passam pelo inglês, então o painel precisa dos dois modelos.

## Quando um idioma não tem modelo

Se um par de idiomas não tiver modelo, o painel informa isso e oferece **Abrir no Seu Navegador em Vez Disso**. Isso abre um site gratuito de tradução, o LibreTranslate (ou o serviço cujo endereço você escolheu na configuração dele), com o seu texto. Só então seu texto sai do seu computador, porque ele vai para esse site, e o painel deixa de dizer que o texto permanece no seu computador. Nada é enviado até que você clique nesse botão.

## Sobre a qualidade

Os modelos são pequenos, então rodam rápido em um computador comum. Eles traduzem bem frases e menos bem rótulos curtos: uma palavra isolada ou o nome de um botão podem sair errados. Leia a tradução antes de usá-la. Nomes, números e tabulações são mantidos como estavam.

Imagens, tabulações e quebras de linha dentro de um parágrafo são deixadas como estão. A tradução preenche apenas as palavras entre elas.

## Relacionados

- [Contar palavras, definir o idioma, verificar a acessibilidade e pesquisar palavras](proofing-tools)
