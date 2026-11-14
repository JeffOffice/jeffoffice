# Traduzir texto
<!-- keywords: traduzir, tradução, tradutor, idioma, idiomas, espanhol, francês, alemão, italiano, português, holandês, russo, ucraniano, indonésio, offline, neste computador, baixar, download, modelos, substituir, inserir abaixo, controlar alterações, controle de alterações -->

**Revisão > Idioma > Traduzir** abre o painel **Traduzir** à direita. Ele traduz o seu texto no seu próprio computador. Suas palavras não são enviadas para lugar nenhum, e não é necessária conexão com a internet depois que os arquivos de tradução de um idioma estiverem no seu computador.

## Traduzir algum texto

1. Selecione o texto que você deseja traduzir. Sem nada selecionado, o painel usa o parágrafo em que está o cursor.
2. Clique em **Traduzir** no grupo **Idioma**. O painel mostra o seu texto em **Original**.
3. Em **De**, escolha o idioma em que o seu texto está. O JeffWrite começa com o idioma marcado no texto. Em **Para**, escolha o idioma desejado. O botão **Trocar** inverte os dois.
4. Clique em **Traduzir**. A tradução aparece em **Tradução**. Palavras em negrito, itálico e sublinhadas continuam em negrito, itálico e sublinhadas, e uma cor ou um tamanho são mantidos onde as palavras tinham a mesma aparência antes.
5. Clique em **Substituir** para colocar a tradução no lugar do seu texto, ou em **Inserir abaixo** para adicioná-la em novos parágrafos sob o seu texto. Qualquer uma das opções é uma única etapa, então **Desfazer** a desfaz. As palavras traduzidas são marcadas com o seu idioma, para que a verificação ortográfica as confira nesse idioma.

Se você mover o cursor ou alterar o texto depois de clicar em **Traduzir**, clique em **Usar seleção** para ler o texto novamente e, em seguida, clique em **Traduzir** de novo. O painel não colocará uma tradução sobre um texto que não seja mais o texto que foi traduzido.

Quando **Controlar Alterações** estiver ativado, **Substituir** e **Inserir abaixo** são alterações controladas, como a sua digitação, para que um revisor possa ver o que havia antes. As palavras traduzidas então assumem a aparência que a sua digitação teria. Palavras que uma alteração controlada excluiu são deixadas de lado: elas não são traduzidas e não são copiadas por **Inserir abaixo**.

**Cancelar** interrompe uma tradução que está demorando demais, e fechar o painel também a interrompe: a janela nunca fica esperando por ela. Textos longos levam mais tempo: uma página de texto leva alguns segundos na maioria dos computadores. O painel traduz no máximo 1.000.000 de letras de uma vez (cerca de 200 páginas); se houver mais selecionado, ele avisa e pede que você selecione menos. Ele divide parágrafos longos em partes de cerca de 1.500 letras, no fim de uma frase sempre que possível, e depois os junta novamente.

## Na primeira vez que você usa um idioma

Uma tradução precisa de um conjunto de arquivos, chamado modelo, para cada direção: um para inglês para alemão, outro para alemão para inglês. Cada modelo tem cerca de 37 MB. Os arquivos não fazem parte do JeffWrite. Na primeira vez que você traduzir com um idioma, o painel informa o que é necessário e o tamanho, por exemplo "Baixar alemão (74 MB)?". Nada é baixado até que você clique nesse botão. O painel mostra o andamento do download, e **Cancelar** interrompe o download e não deixa nada para trás.

O JeffWrite confere cada arquivo com uma impressão digital (uma soma de verificação SHA-256) antes de usá-lo. Um arquivo que não corresponda é descartado, e o idioma não é instalado.

Os arquivos ficam em uma única pasta que todos os aplicativos do JeffOffice compartilham (uma pasta chamada JeffOffice na pasta de dados do seu usuário, em Translation/models), então um idioma baixado em um aplicativo fica pronto nos outros, e é baixado apenas uma vez. Dois idiomas que não têm modelo próprio, como alemão e francês, passam pelo inglês, então o painel precisa dos dois modelos.

## Quando um idioma não tem modelo

Se um par de idiomas não tiver modelo, o painel informa isso e oferece **Abrir no seu navegador em vez disso**. Isso abre um site de tradução gratuito, o LibreTranslate (ou o serviço cujo endereço você escolheu na configuração correspondente), com o seu texto. Só então o seu texto sai do seu computador, porque é enviado para esse site, e o painel deixa de dizer que ele permanece no seu computador. Nada é enviado até que você clique nesse botão.

## Sobre a qualidade

Os modelos são pequenos, então funcionam rápido em um computador comum. Eles traduzem bem frases e menos bem rótulos curtos: uma única palavra ou o nome de um botão pode sair errado. Leia a tradução antes de usá-la. Nomes, números e tabulações são mantidos como estavam.

Imagens, tabulações e quebras de linha dentro de um parágrafo são deixadas como estão. A tradução preenche apenas as palavras entre eles.

## Veja também

- [Contar palavras, definir o idioma, verificar a acessibilidade e pesquisar palavras](proofing-tools)
